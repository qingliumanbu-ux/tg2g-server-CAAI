/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:     
//修改日期:  
//函数说明：对生产的投料进行按产量分摊，暂定生产实绩的为RULE_TYPE = '1'，分摊规则按批次号的产量进行分摊。
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

BM2_FUNCTION_EXPORT
int f_caai_sj(CString stats_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;	

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");	
	CDecimal all_wt ,ft_wt,use_all_wt;

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{

		//产出
		sqlstr = " delete from tcaai02"
			" where 1=1"
			" AND RULE_TYPE = '1'"
			" AND DEPT_CODE =@dept_code"
			" AND STATS_PERIOD = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//优化使用脚本运行
		sqlstr = " insert into tcaai02 (REC_CREATE_TIME,dept_code,cost_center,stats_period,RELATION_NO,product_code,MAT_CODE,RULE_TYPE,WT"
			"  ,prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, vfree1, vfree2, vfree3, vfree4, vfree5 )"
			" select @datenow,dept_code,t1.cost_center,stats_period,t1.RELATION_NO,product_code,mat_code,'1',decode(all_wt,0,0,ROUND(WT*use_wt/all_wt,4))"
			"  ,prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, vfree1, vfree2, vfree3, vfree4, vfree5 "
			" FROM "
			" (select dept_code, cost_center, stats_period, RELATION_NO, MAT_CODE product_code, SUM(QTY) QTY, SUM(WT) WT"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, vfree1, vfree2, vfree3, vfree4, vfree5"
			" from tcaais1"
			" where 1 = 1"
			" AND PRO_FLAG = 'O'"
			" AND DEPT_CODE = @dept_code"
			" AND STATS_PERIOD = @stats_period"
			" group by dept_code, cost_center, stats_period, relation_no, MAT_CODE"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, vfree1, vfree2, vfree3, vfree4, vfree5"
			" ) t1"
			" left join"
			" (select cost_center, relation_no, SUM(WT) all_wt"
			" from tcaais1"
			" where 1 = 1"
			" AND PRO_FLAG = 'O'"
			" AND DEPT_CODE = @dept_code"
			" AND STATS_PERIOD = @stats_period"
			" GROUP BY cost_center, relation_no) t2 on t1.cost_center = t2.cost_center and t1.relation_no = t2.relation_no"
			" left join"
			" (select mat_code, cost_center, relation_no, sum(wt) use_wt"
			" from tcaais1"
			" where 1 = 1"
			" AND PRO_FLAG = 'I'"
			" AND DEPT_CODE = @dept_code"
			" AND STATS_PERIOD = @stats_period"
			" GROUP BY MAT_CODE, cost_center, relation_no)  t3 on t1.cost_center = t3.cost_center and t1.relation_no = t3.relation_no"
			" where  decode(mat_code, '', ' ', mat_code) != ' '"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//误差调整
		sqlstr = "select mat_code,cost_center,relation_no,sum(all_wt-use_wt)"
			" from ("
			"select mat_code, cost_center, relation_no, sum(wt) all_wt,0 use_wt"
			" from tcaais1"
			" where 1 = 1"
			" AND PRO_FLAG = 'I'"
			" AND DEPT_CODE = @dept_code"
			" AND STATS_PERIOD = @stats_period"
			" GROUP BY MAT_CODE, cost_center, relation_no"
			" union all"
			" select mat_code, cost_center, relation_no,0 all_wt, sum(wt) use_wt"
			" from tcaai02"
			" where 1 = 1"
			" AND RULE_TYPE = '1'"
			" AND DEPT_CODE = @dept_code"
			" AND STATS_PERIOD = @stats_period"
			" GROUP BY MAT_CODE, cost_center, relation_no"
			" ) GROUP BY MAT_CODE, cost_center, relation_no"
			" having sum(all_wt-use_wt)!=0"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				sqlstr = " update tcaai02 set wt = wt+@dif_wt"
					" where 1=1"
					" and wt = (select max(wt) from tcaai02 where 1=1 AND mat_code = @mat_code and relation_no = @relation_no AND cost_center = @cost_center AND RULE_TYPE = '1' AND DEPT_CODE = @dept_code AND STATS_PERIOD = @stats_period)"
					" AND mat_code = @mat_code"
					" and relation_no = @relation_no"
					" AND cost_center = @cost_center"
					" AND RULE_TYPE = '1'"
					" AND DEPT_CODE = @dept_code"
					" AND STATS_PERIOD = @stats_period"
					" FETCH FIRST 1 ROWS  ONLY"
					;
				break;
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " update tcaai02 set wt = wt+@dif_wt"
					" where 1=1"
					" and wt = (select max(wt) from tcaai02 where 1=1 AND mat_code = @mat_code and relation_no = @relation_no AND cost_center = @cost_center AND RULE_TYPE = '1' AND DEPT_CODE = @dept_code AND STATS_PERIOD = @stats_period)"
					" AND mat_code = @mat_code"
					" and relation_no = @relation_no"
					" AND cost_center = @cost_center"
					" AND RULE_TYPE = '1'"
					" AND DEPT_CODE = @dept_code"
					" AND STATS_PERIOD = @stats_period"
					" and rownum = 1"
					;
				break;
			}
			
			//Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(2));
			cmd_inq_1.Parameters.Set("relation_no", cmd_inq.GetString(3));
			cmd_inq_1.Parameters.Set("dif_wt", cmd_inq.GetDecimal(4));
			cmd_inq_1.Parameters.Set("stats_period", stats_period);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);			
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();
		}
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1, 1, "[%s]", s.sysmsg);
		s.flag = -1;
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
	return doFlag;
}
