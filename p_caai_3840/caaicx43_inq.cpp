/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2018-06-21
Description:查询工序产品成本明细
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx43_inq)
//-EP_SYSTEM_HEAD_END

int f_caaicx43_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString		begin_time = " ";
	CString		end_time = " ";
	CString		mat_code = " ";
	CString		back_code_2 = " ";
	CString		dept_code = " ";
	CString		flag = "0";
	int rownum = 1;
	CDecimal wt2 = 0;
	CModel tcaai04("TCAAI04");


	CString  sqlstr("");
	CString  sqlstr_where("");
	CString  sqlstr_sub("");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		flag = bcls_rec->Tables[0].Rows[0]["FLAG"].ToString(); //用来判断是单耗还是消耗
		tcaai04.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		//条件
		sqlstr_where = " ";
		if (tcaai04["SG_SIGN"].ToString().Trim() != "")
		{
			sqlstr_where += " and sg_sign = @sg_sign ";
		}		
		if (tcaai04["VFREE1"].ToString().Trim() != "")
		{
			sqlstr_where += " AND VFREE1=@vfree1 ";
		}
		if (tcaai04["MAT_THICK"].ToDecimal() != 0)
		{
			sqlstr_where += " AND MAT_THICK=@mat_thick ";
		}
		sqlstr_where += " and dept_code = @dept_code"
			" and stats_period between @begin_time and @end_time"
			;


		//1、各明细消耗项的消耗字段查询,扣除钢坯的信息（暂定钢坯的物料代码字段长度超过20字符的）如SUM(case when mat_code= 'NY001' then mat_use_1 else 0 end)  as 'NY001_USE'
		sqlstr = " select  mat_code"
			" from tcaai04 "
			" WHERE	1=1 "
			" and mat_code !=' '"
			" and length(mat_code)<20"
			;
		sqlstr = sqlstr + sqlstr_where;
		sqlstr += " group by mat_code";

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("sg_sign", tcaai04["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaai04["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("vfree1", tcaai04["VFREE1"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai04["DEPT_CODE"].ToString());
		cmd_inq.ExecuteReader();
		sqlstr_sub = "";
		while (cmd_inq.Read())
		{
			mat_code = cmd_inq.GetString(1);
			sqlstr = " select mat_type FROM TCAAC11"
				" WHERE mat_type in ('B','C','E')"
				" and MAT_CODE = @mat_code"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("mat_code", mat_code);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				if (flag == "1")
				{
					sqlstr_sub += ",SUM(case when mat_code= '" + mat_code + "' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) else 0 end)  as " + mat_code + "_USE";
				}
				else
				{
					sqlstr_sub += ",SUM(case when mat_code= '" + mat_code + "' then COST_YJ else 0 end)  as " + mat_code + "_USE";
				}

			}
			else
			{
				if (flag == "1")
				{
					sqlstr_sub += ",SUM(case when mat_code= '" + mat_code + "' and OUTPUT!=0 then round(WT/OUTPUT,6) else 0 end)  as " + mat_code + "_USE";					
				}
				else
				{
					sqlstr_sub += ",SUM(case when mat_code= '" + mat_code + "' then WT else 0 end)  as " + mat_code + "_USE";
				}
			}

			rownum++;
			if (rownum == 100)
			{
				break;
			}
		}
		cmd_inq.Close();

		if (tcaai04["SUB_BACKLOG_CODE"].ToString().Trim() != "")
		{
			//拼接sql		
			sqlstr = " select t1.vfree1,t1.SUB_BACKLOG_CODE,t1.EQU_NO,t1.PRODUCT_CODE,t1.MAT_THICK,t1.SG_SIGN,t1.STATS_PERIOD as prod_date,t1.PROD_SHIFT_GROUP,t1.PROD_SHIFT_NO"
				",t1.wt,t1.COST_YJ,decode(t1.wt,0,0,round(t1.COST_YJ/t1.wt,6)) PRICE_UNIT_GJ"
				",t3.ZHIZAO_USE,t3.HUISHOU_USE,t3.DONGLI_USE,t3.GONGZI_USE"
				" ,t2.*"
				" ,case when instr(t4.BACKLOG_EA,'R')>0 then '1' else '0' end RH_FLAG"
				" from "
				//产量
				" (select vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO,sum(WT) as WT,SUM(COST_YJ) COST_YJ"
				" from tcaai03"
				" where 1=1"
				" AND SUB_BACKLOG_CODE = @sub_backlog_code "
				+ sqlstr_where +
				" group by vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO) t1"
				" left join " //明细项消耗
				" (select vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO"
				+ sqlstr_sub +
				" from tcaai04"
				" where 1=1"
				" AND SUB_BACKLOG_CODE = @sub_backlog_code "
				+ sqlstr_where +
				" group by vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO) t2 "
				" on t1.vfree1 = t2.vfree1 AND t1.SUB_BACKLOG_CODE =  t2.SUB_BACKLOG_CODE and t1.EQU_NO = t2.EQU_NO and t1.PRODUCT_CODE=t2.PRODUCT_CODE"
				" AND t1.MAT_THICK =  t2.MAT_THICK AND t1.SG_SIGN =  t2.SG_SIGN and t1.STATS_PERIOD=t2.stats_period and t1.PROD_SHIFT_GROUP=t2.PROD_SHIFT_GROUP and t1.PROD_SHIFT_NO=t2.PROD_SHIFT_NO"
				" left join " //制造费用
				" (select vfree1,t.SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO"
				",sum(case when BACK_CODE_2 ='C' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='C' and @flag !='1' then COST_YJ else 0 end) ZHIZAO_USE"
				" ,sum(case when BACK_CODE_2 ='D' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='D' and @flag !='1' then COST_YJ  else 0 end) HUISHOU_USE"
				" ,sum(case when BACK_CODE_2 ='A' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='A' and @flag !='1' then COST_YJ  else 0 end) DONGLI_USE"
				" ,sum(case when BACK_CODE_2 ='B' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='B' and @flag !='1' then COST_YJ  else 0 end) GONGZI_USE"
				" from tcaai04 t"
				" left join tcaac03  t03 on t.mat_code = t03.mat_code and t.sub_backlog_code = t03.sub_backlog_code"
				" where 1=1"
				" AND t.SUB_BACKLOG_CODE = @sub_backlog_code "
				+ sqlstr_where +
				" group by vfree1,t.SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO) t3 "
				" on t1.vfree1 = t3.vfree1 AND t1.SUB_BACKLOG_CODE =  t3.SUB_BACKLOG_CODE and t1.EQU_NO = t3.EQU_NO and t1.PRODUCT_CODE=t3.PRODUCT_CODE"
				" AND t1.MAT_THICK =  t3.MAT_THICK AND t1.SG_SIGN =  t3.SG_SIGN and t1.STATS_PERIOD=t2.stats_period and t1.PROD_SHIFT_GROUP=t2.PROD_SHIFT_GROUP and t1.PROD_SHIFT_NO=t2.PROD_SHIFT_NO"
				" left join tpssm41 t4 on t1.vfree1 = t4.heat_no"
				;
		}
		else
		{
			sqlstr = " select t1.vfree1,t1.PRODUCT_CODE,t1.MAT_THICK,t1.SG_SIGN,t1.STATS_PERIOD as prod_date,t1.PROD_SHIFT_GROUP,t1.PROD_SHIFT_NO"
				",t1.wt,t1.COST_YJ,decode(t1.wt,0,0,round(t1.COST_YJ/t1.wt,6)) PRICE_UNIT_GJ"
				",t3.ZHIZAO_USE,t3.HUISHOU_USE,t3.DONGLI_USE,t3.GONGZI_USE"
				" ,t2.*"
				" ,case when instr(t4.BACKLOG_EA,'R')>0 then '1' else '0' end RH_FLAG"
				" from "
				//产量
				" (select vfree1,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO"
				",sum(case when sub_backlog_code = 'C' then WT else 0 end) as WT,SUM(COST_YJ) COST_YJ"
				" from tcaai03"
				" where 1=1"
				+ sqlstr_where +
				" group by vfree1,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO) t1"
				" left join " //明细项消耗
				" (select vfree1,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO"
				+ sqlstr_sub +
				" from tcaai04"
				" where 1=1"
				+ sqlstr_where +
				" group by vfree1,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO) t2 "
				" on t1.vfree1 = t2.vfree1  and t1.PRODUCT_CODE=t2.PRODUCT_CODE"
				" AND t1.MAT_THICK =  t2.MAT_THICK AND t1.SG_SIGN =  t2.SG_SIGN and t1.STATS_PERIOD=t2.stats_period and t1.PROD_SHIFT_GROUP=t2.PROD_SHIFT_GROUP and t1.PROD_SHIFT_NO=t2.PROD_SHIFT_NO"
				" left join " //制造费用
				" (select vfree1,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO"
				",sum(case when BACK_CODE_2 ='C' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='C' and @flag !='1' then COST_YJ else 0 end) ZHIZAO_USE"
				" ,sum(case when BACK_CODE_2 ='D' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='D' and @flag !='1' then COST_YJ  else 0 end) HUISHOU_USE"
				" ,sum(case when BACK_CODE_2 ='A' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='A' and @flag !='1' then COST_YJ  else 0 end) DONGLI_USE"
				" ,sum(case when BACK_CODE_2 ='B' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='B' and @flag !='1' then COST_YJ  else 0 end) GONGZI_USE"
				" ,sum(case when BACK_CODE_2 ='E' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='E' and @flag !='1' then COST_YJ  else 0 end) ZHAGUN_USE"
				" from tcaai04 t"
				" left join tcaac03  t03 on t.mat_code = t03.mat_code and t.sub_backlog_code = t03.sub_backlog_code"
				" where 1=1"
				+ sqlstr_where +
				" group by vfree1,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,PROD_SHIFT_GROUP,PROD_SHIFT_NO) t3 "
				" on t1.vfree1 = t3.vfree1  and t1.PRODUCT_CODE=t3.PRODUCT_CODE"
				" AND t1.MAT_THICK =  t3.MAT_THICK AND t1.SG_SIGN =  t3.SG_SIGN and t1.STATS_PERIOD=t3.stats_period and t1.PROD_SHIFT_GROUP=t3.PROD_SHIFT_GROUP and t1.PROD_SHIFT_NO=t3.PROD_SHIFT_NO"
				" left join tpssm41 t4 on t1.vfree1 = t4.heat_no"
				;
		}
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("flag", flag);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("sg_sign", tcaai04["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaai04["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("vfree1", tcaai04["VFREE1"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai04["DEPT_CODE"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}