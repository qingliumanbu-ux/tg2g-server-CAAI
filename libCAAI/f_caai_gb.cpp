/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:     
//修改日期:  
//函数说明：对收集归来的合格数据进行归并汇总
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
BM2_FUNCTION_EXPORT
int f_caai_gb(CString stats_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;	

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	
	// 创建电文处理对象
	CDbCommand cmd_inq(conn);

	try
	{
		//产出
		sqlstr = " delete from tcaais1"
			" where 1=1"
			" AND DEPT_CODE like trim(@dept_code)||'%'"
			" AND stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = "insert into tcaais1(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,RELATION_NO,PRO_FLAG,MAT_CODE,PRODUCT_FLAG,WCE,QTY,WT"
			", prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 )"
			" SELECT @datenow, DEPT_CODE, STATS_PERIOD, COST_CENTER,RELATION_NO,PRO_FLAG,MAT_CODE,PRODUCT_FLAG,WCE,SUM(QTY),SUM(WT)"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
			" from "
			" (SELECT  DEPT_CODE, STATS_PERIOD, COST_CENTER,RELATION_NO,PRO_FLAG,MAT_CODE,PRODUCT_FLAG,WCE,QTY,WT"
			" , STATS_PERIOD as prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
			" FROM tcaaia1"
			" WHERE 1=1"
			" and CHECK_FLAG = '1'"
			" AND DEPT_CODE like trim(@dept_code)||'%'"
			" AND stats_period = @stats_period"
			")"
			" group by  DEPT_CODE, STATS_PERIOD, COST_CENTER,RELATION_NO,PRO_FLAG,MAT_CODE,PRODUCT_FLAG,WCE,vfree1,vfree2,vfree3,vfree4,vfree5"
			" ,prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5"
			;
		//Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//插入按产量分摊的分摊基数信息
		//1、产量法
		sqlstr = " delete from tcaai01"
			" where 1=1"
			" AND DEPT_CODE = @dept_code"
			" and divvy_type in ('1','3')"
			" and STATS_PERIOD = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n,VFREE1,VFREE2,VFREE3,VFREE4,VFREE5 )"
			" SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'3',SUM(WT),VFREE1,VFREE2,VFREE3,VFREE4,VFREE5"
			" FROM TCAAIS1"
			" WHERE 1=1"
			" AND DEPT_CODE = @dept_code"
			" AND PRO_FLAG ='O' " //产出工序
			" AND stats_period = @stats_period"
			" group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,VFREE1,VFREE2,VFREE3,VFREE4,VFREE5 "
			" HAVING SUM(WT)!=0"
			;
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();		

		//更新产量
		sqlstr = " update tcaaia14 t1 set wt = (select sum(WT) from tcaais1 t2 where t1.plan_no = t2.vfree1 and t1.equ_no = t2.equ_no AND t2.PRO_FLAG ='O' and t2.dept_code = @dept_code and t2.stats_period=@stats_period )"
			" where 1=1"
			" and exists (select 1 from tcaais1 t2 where t1.plan_no = t2.vfree1 and t1.equ_no = t2.equ_no AND t2.PRO_FLAG ='O' and t2.dept_code = @dept_code and t2.stats_period=@stats_period )"
			" and dept_code =@dept_code"
			" and stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//工时分摊
		sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n,VFREE1,VFREE2,VFREE3,VFREE4,VFREE5 )"
			" SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, t1.equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'1',round(sum(use_wt*wt/total_wt),3),t1.VFREE1,VFREE2,VFREE3,VFREE4,VFREE5"
			" FROM TCAAIS1 t1"
			" LEFT JOIN "
			" (SELECT VFREE1,sub_backlog_code, equ_no,SUM(WT) total_wt FROM TCAAIS1"
			" WHERE 1=1"
			" AND PRO_FLAG ='O' " //产出工序
			" AND DEPT_CODE = @dept_code"
			" AND stats_period = @stats_period"
			" GROUP BY VFREE1,sub_backlog_code, equ_no"
			" ) t2 on t1.VFREE1=t2.VFREE1 and t1.sub_backlog_code=t2.sub_backlog_code and t1.equ_no = t2.equ_no"
			" left join "
			" (select  RELATION_NO,equ_no,sum(ONOFF_TIME) use_wt from tcaaia14 t"
			" WHERE 1=1"
			" AND DEPT_CODE = @dept_code"
			" AND stats_period = @stats_period"
			" group by RELATION_NO,equ_no) t3 on t1.VFREE1 =t3.RELATION_NO and t1.equ_no = t3.equ_no"
			" WHERE 1=1"
			" and nvl(total_wt,0)!=0"
			" and nvl(use_wt,0)!=0"
			" AND t1.PRO_FLAG ='O' " //产出工序
			" AND t1.DEPT_CODE = @dept_code"
			" AND t1.stats_period = @stats_period"
			" group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, t1.equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,t1.VFREE1,VFREE2,VFREE3,VFREE4,VFREE5 "
			" HAVING  sum(wt)!=0"
			;
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//插入能源的消耗
		if (dept_code == "S")
		{		
		sqlstr = "insert into tcaais1(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,RELATION_NO,PRO_FLAG,MAT_CODE,WT, sub_backlog_code, equ_no)"
			" SELECT @datenow, DEPT_CODE, STATS_PERIOD, sub_backlog_code,RELATION_NO,'I' PRO_FLAG,MAT_CODE,SUM(WT), sub_backlog_code, equ_no "
			" from "
			" (SELECT  t1.DEPT_CODE, t1.STATS_PERIOD, t1.SUB_BACKLOG_CODE,t1.RELATION_NO, t1.equ_no,t12.MAT_CODE,COMSUME_WT as wt"
			" FROM tcaaia14 t1"
			" left join tcaaia12 t12 on t12.plan_no = decode(t1.dept_code,'S',t1.pono,t1.RELATION_NO) and t12.equ_no = t1.equ_no and t12.busi_type = 'SC'"
			" WHERE 1=1"
			" and mat_code !=' '"
			" AND NVL(COMSUME_WT,0)!=0"
			" AND t1.DEPT_CODE =@dept_code"
			" AND t1.stats_period = @stats_period"
			")"			
			" group by  DEPT_CODE, STATS_PERIOD, SUB_BACKLOG_CODE,RELATION_NO,MAT_CODE,equ_no "	
			" HAVING SUM(WT)!=0"
			;
		Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		}
		else
		{
			sqlstr = "insert into tcaais1(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,RELATION_NO,PRO_FLAG,MAT_CODE,WT, sub_backlog_code, equ_no)"
				" SELECT @datenow, DEPT_CODE, STATS_PERIOD, sub_backlog_code,RELATION_NO,'I' PRO_FLAG,MAT_CODE,SUM(WT), sub_backlog_code, equ_no "
				" from "
				" (SELECT  t1.DEPT_CODE, t1.STATS_PERIOD, t1.SUB_BACKLOG_CODE,t1.RELATION_NO, t1.equ_no,t12.MAT_CODE,COMSUME_WT as wt"
				" FROM tcaaia14 t1"
				" left join tcaaia12 t12 on t12.plan_no =t1.RELATION_NO and  t12.busi_type = 'SC'"
				" WHERE 1=1"
				" AND NVL(COMSUME_WT,0)!=0"
				" AND t1.DEPT_CODE =@dept_code"
				" AND t1.stats_period = @stats_period"
				")"
				" group by  DEPT_CODE, STATS_PERIOD, SUB_BACKLOG_CODE,RELATION_NO,MAT_CODE,equ_no "
				" HAVING SUM(WT)!=0"
				;
			Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

		}

		if (dept_code == "X")  //副产品回收
		{
			sqlstr = "insert into tcaais1(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,RELATION_NO,PRO_FLAG,MAT_CODE,WT, sub_backlog_code, equ_no)"
				" SELECT @datenow, DEPT_CODE, STATS_PERIOD, sub_backlog_code,RELATION_NO,'I' PRO_FLAG,MAT_CODE,0-SUM(WT), sub_backlog_code, equ_no "
				" from "
				" (SELECT  t1.DEPT_CODE, t1.STATS_PERIOD, t1.SUB_BACKLOG_CODE,t1.RELATION_NO, t1.equ_no,t12.MAT_CODE,PROD_WT as wt"
				" FROM tcaaia14 t1"
				" left join tcaaia8 t12 on t12.plan_no = t1.RELATION_NO and t12.prod_date = @stats_period"
				" WHERE 1=1"
				" AND NVL(PROD_WT,0)!=0"
				" AND t1.DEPT_CODE =@dept_code"
				" AND t1.stats_period = @stats_period"
				")"
				" group by  DEPT_CODE, STATS_PERIOD, SUB_BACKLOG_CODE,RELATION_NO,MAT_CODE,equ_no "
				" HAVING SUM(WT)!=0"
				;
			Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}

		//将定额耗用的数据放入FT表中
		sqlstr = " delete from tcaais2"
			" where 1=1"
			" AND data_from ='DE' " //定额消耗的
			" AND DEPT_CODE =@dept_code "
			" AND STATS_PERIOD = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//定额使用--按天
		sqlstr = " insert into tcaais2 ( rec_creator, rec_create_time, rule_type, data_from, mat_code, mat_name, stats_period, dept_code,sub_backlog_code,  cost_center,wt,amt,prod_date,equ_no,sg_sign,mat_thick,mat_width,mat_len)"
			" select @rec_creator, @rec_create_time,decode(use_unit,0,'3','2') ,'DE',mat_code,' ',@stats_period,@dept_code,cost_center,cost_center,use_unit, unit_cost,@stats_period,equ_no,sg_sign,mat_thick,mat_width,mat_len "
			" from tcaac10"
			" where 1=1"
			" and (use_unit!=0 or unit_cost!=0)"
			" and type_code = 'day'"
			" and cost_center in (select distinct sub_backlog_code  from tcaais1"
			" where 1=1 "
			" AND PRO_FLAG ='O' " //产出工序
			" AND DEPT_CODE = @dept_code"
			" AND stats_period = @stats_period"
			")"
			//" and dept_code =@dept_code"			
			;
		Log::Trace("", "", "定额1sqlstr={0}", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " insert into tcaais2 ( rec_creator, rec_create_time, rule_type, data_from, mat_code, mat_name, stats_period, dept_code,sub_backlog_code,  cost_center,wt,amt,prod_date,equ_no,sg_sign,mat_thick,mat_width,mat_len)"
			" select @rec_creator, @rec_create_time,decode(use_unit,0,'3','2') ,'DE',mat_code,' ',@stats_period,@dept_code,cost_center,cost_center,use_unit*out_wt, unit_cost*out_wt,@stats_period,equ_no,sg_sign,mat_thick,mat_width,mat_len "
			" from tcaac10 t1"
			" left join"
			" (select sub_Backlog_code,sum(wt) out_wt from tcaais1 "
			" where 1=1 "
			" AND PRO_FLAG ='O' " //产出工序
			" AND DEPT_CODE = @dept_code"
			" AND stats_period = @stats_period"
			" group by sub_Backlog_code"
			") t2 on t2.sub_Backlog_Code = t1.cost_center"
			" where 1=1"
			" and (use_unit!=0 or unit_cost!=0)"
			" AND NVL(out_wt,0) !=0"
			" and type_code = 'unit'"
			" and dept_code =@dept_code"

			;
		Log::Trace("", "", "定额2sqlstr={0}", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
